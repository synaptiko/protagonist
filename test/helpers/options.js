const assert = require('chai').assert;
const path = require('path');
const fs = require('fs');

const valid_fixture = fs.readFileSync(path.join(__dirname, '..', 'fixtures', 'valid.apib'), 'utf8');

const valid_refract = require('../fixtures/valid.parse.json');
const valid_sourcemap_refract = require('../fixtures/valid.parse.sourcemap.json');
const require_name = require('../fixtures/require-name.json');
const generate_body = require('../fixtures/generate-body.parse.json');
const generate_body_sourcemap = require('../fixtures/generate-body.parse.sourcemap.json');
const generate_body_schema = require('../fixtures/generate-body-schema.parse.json');
const generate_body_schema_sourcemap = require('../fixtures/generate-body-schema.parse.sourcemap.json');

module.exports = (parser) => {
  describe('Options containing', () => {
    it('unsupported option when parsing should throw', () => {
      assert.throws(() => {
        parser.parse(valid_fixture, { type: 'refract' });
      }, `unrecognized option 'type', expected: 'requireBlueprintName', 'generateMessageBody', 'generateMessageBodySchema', 'generateSourceMap'`);
    });

    it('unsupported option when validating should throw', () => {
      assert.throws(() => {
        parser.validate(valid_fixture, { type: 'refract' });
      }, `unrecognized option 'type', expected: 'requireBlueprintName'`);
    });

    it('sourcemap option set to false should work', (done) => {
      parser.parse(valid_fixture, { generateSourceMap: false })
        .then((refract) => {
          assert.deepEqual(refract, valid_refract);
          done();
        })
        .catch(done);
    })

    it('old sourcemap option should work', (done) => {
      parser.parse(valid_fixture, { exportSourcemap: true })
        .then((refract) => {
          assert.deepEqual(refract, valid_sourcemap_refract);
          done();
        })
        .catch(done);
    });

    describe('requireBlueprintName', () => {
      describe('when parsing', () => {
        it('should work', (done) => {
          parser.parse(valid_fixture, { requireBlueprintName: true })
            .then((refract) => {
              assert.deepEqual(refract, require_name);
              done();
            })
            .catch(done);
        });

        it('along with sourcemap should work', (done) => {
          parser.parse(valid_fixture, { requireBlueprintName: true, generateSourceMap: true })
            .then((refract) => {
              assert.deepEqual(refract, require_name);
              done();
            })
            .catch(done);
        })
      });

      it('when validating should work', (done) => {
        parser.validate(valid_fixture, { requireBlueprintName: true })
          .then((refract) => {
            assert.deepEqual(refract, require_name);
            done();
          })
          .catch(done);
      });
    });

    describe('generateMessageBody', () => {
      describe('when parsing', () => {
        it('should work', (done) => {
          parser.parse(valid_fixture, { generateMessageBody: false })
            .then((refract) => {
              assert.deepEqual(refract, generate_body);
              done();
            })
            .catch(done);
        });

        it('along with sourcemap should work', (done) => {
          parser.parse(valid_fixture, { generateMessageBody: false, generateSourceMap: true })
            .then((refract) => {
              assert.deepEqual(refract, generate_body_sourcemap);
              done();
            })
            .catch(done);
        })
      });
    });

    describe('generateMessageBodySchema', () => {
      describe('when parsing', () => {
        it('should work', (done) => {
          parser.parse(valid_fixture, { generateMessageBodySchema: false })
            .then((refract) => {
              assert.deepEqual(refract, generate_body_schema);
              done();
            })
            .catch(done);
        });

        it('along with sourcemap should work', (done) => {
          parser.parse(valid_fixture, { generateMessageBodySchema: false, generateSourceMap: true })
            .then((refract) => {
              assert.deepEqual(refract, generate_body_schema_sourcemap);
              done();
            })
            .catch(done);
        })
      });
    });
  });
};
